#!/usr/bin/env python3

import argparse
import os
import sys
import textwrap

from kconfiglib import Kconfig, BOOL, TRISTATE, TRI_TO_STR


# Warnings that won't be turned into errors (but that will still be printed),
# identified by a substring of the warning. The warning texts from Kconfiglib
# are guaranteed to not change.
WARNING_WHITELIST = (
    # Warning generated when a symbol with unsatisfied dependencies is being
    # selected. These should be investigated, but whitelist them for now.
    "y-selected",
)


def fatal(warning):
    # Returns True if 'warning' is not whitelisted and should be turned into an
    # error

    return not any(wl_warning in warning for wl_warning in WARNING_WHITELIST)


def main():
    args = parse_args()

    print("Parsing Kconfig tree in " + args.kconfig_root)
    kconf = Kconfig(args.kconfig_root, warn_to_stderr=False,
                    suppress_traceback=True)

    # Warn for assignments to undefined symbols
    kconf.warn_assign_undef = True

    # prj.conf may override settings from the board configuration, so disable
    # warnings about symbols being assigned more than once
    kconf.warn_assign_override = False
    kconf.warn_assign_redun = False

    print(kconf.load_config(args.conf_fragments[0]))
    for config in args.conf_fragments[1:]:
        # replace=False creates a merged configuration
        print(kconf.load_config(config, replace=False))

    if not os.path.exists(args.autoconf):
        # If zephyr/.config does not exist, it means we just merged
        # configuration fragments. Check that there were no assignments to
        # promptless symbols in them. Such assignments have no effect.
        #
        # This won't work if zephyr/.config already exists (which means it's
        # being loaded), because zephyr/.config is a full configuration file
        # that includes values for promptless symbols.
        check_no_promptless_assign(kconf)

    # Print warnings for symbols whose actual value doesn't match the assigned
    # value
    for sym in kconf.unique_defined_syms:
        # Was the symbol assigned to? Choice symbols are checked separately.
        if sym.user_value is not None and not sym.choice:
            check_assigned_sym_value(sym)

    # Print warnings for choices whose actual selection doesn't match the user
    # selection
    for choice in kconf.unique_choices:
        if choice.user_selection:
            check_assigned_choice_value(choice)

    # Hack: Force all symbols to be evaluated, to catch warnings generated
    # during evaluation. Wait till the end to write the actual output files, so
    # that we don't generate any output if there are warnings-turned-errors.
    #
    # Kconfiglib caches calculated symbol values internally, so this is still
    # fast.
    kconf.write_config(os.devnull)

    # Print warnings ourselves so that we can put a blank line between them for
    # readability. We could roll this into the loop below, but it's nice to
    # always print all warnings, even if one of them turns out to be fatal.
    for warning in kconf.warnings:
        print("\n" + warning, file=sys.stderr)

    # Turn all warnings except for explicitly whitelisted ones into errors. In
    # particular, this will turn assignments to undefined Kconfig variables
    # into errors.
    #
    # A warning is generated by this script whenever a symbol gets a different
    # value than the one it was assigned. Keep that one as just a warning for
    # now as well.
    for warning in kconf.warnings:
        if fatal(warning):
            sys.exit("\n" + textwrap.fill(
                "Error: Aborting due to non-whitelisted Kconfig "
                "warning '{}'.\nNote: If this warning doesn't point "
                "to an actual problem, you can add it to the "
                "whitelist at the top of {}."
                .format(warning, sys.argv[0]),
                100) + "\n")

    # Write the merged configuration and the C header
    print(kconf.write_config(args.dotconfig))
    kconf.write_autoconf(args.autoconf)

    # Write the list of processed Kconfig sources to a file
    write_kconfig_filenames(kconf.kconfig_filenames, kconf.srctree, args.sources)


def check_no_promptless_assign(kconf):
    # Checks that no promptless symbols are assigned

    for sym in kconf.unique_defined_syms:
        if sym.user_value is not None and promptless(sym):
            err(("""\
{0.name_and_loc} is assigned in a configuration file, but is not
directly user-configurable (has no prompt). It gets its value indirectly from
other symbols. \
""" + SYM_INFO_HINT).format(sym))


def check_assigned_sym_value(sym):
    # Verifies that the value assigned to 'sym' "took" (matches the value the
    # symbol actually got), printing a warning otherwise

    # Tristate values are represented as 0, 1, 2. Having them as
    # "n", "m", "y" is more convenient here, so convert.
    if sym.type in (BOOL, TRISTATE):
        user_value = TRI_TO_STR[sym.user_value]
    else:
        user_value = sym.user_value

    if user_value != sym.str_value:
        warn(("""\
{0.name_and_loc} was assigned the value '{1}' but got the value
'{0.str_value}'. Check its dependencies. \
""" + SYM_INFO_HINT).format(sym, user_value))


def check_assigned_choice_value(choice):
    # Verifies that the choice symbol that was selected (by setting it to y)
    # ended up as the selection, printing a warning otherwise.
    #
    # We check choice symbols separately to avoid warnings when two different
    # choice symbols within the same choice are set to y. This might happen if
    # a choice selection from a board defconfig is overridden in a prj.conf, for
    # example. The last choice symbol set to y becomes the selection (and all
    # other choice symbols get the value n).
    #
    # Without special-casing choices, we'd detect that the first symbol set to
    # y ended up as n, and print a spurious warning.

    if choice.user_selection is not choice.selection:
        warn(("""\
the choice symbol {0.name_and_loc} was selected (set =y), but {1} ended up as
the choice selection. \
""" + SYM_INFO_HINT).format(
            choice.user_selection,
            choice.selection.name_and_loc if choice.selection else "no symbol"))


# Hint on where to find symbol information. Expects the first argument of
# format() to be the symbol.
SYM_INFO_HINT = """\
See http://docs.zephyrproject.org/latest/reference/kconfig/CONFIG_{0.name}.html
and/or look up {0.name} in the menuconfig/guiconfig interface. The Application
Development Primer, Setting Configuration Values, and Kconfig - Tips and Best
Practices sections of the manual might be helpful too.\
"""


def promptless(sym):
    # Returns True if 'sym' has no prompt. Since the symbol might be defined in
    # multiple locations, we need to check all locations.

    return not any(node.prompt for node in sym.nodes)


def write_kconfig_filenames(paths, root_path, output_file_path):
    # 'paths' is a list of paths. The list has duplicates and the
    # paths are either absolute or relative to 'root_path'.

    # We need to write this list, in a format that CMake can easily
    # parse, to the output file at 'output_file_path'.

    # The written list has sorted real (absolute) paths, and it does not have
    # duplicates. The list is sorted to be deterministic. It is realpath()'d
    # to ensure that different representations of the same path does not end
    # up with two entries, as that could cause the build system to fail.

    paths_uniq = sorted({os.path.realpath(os.path.join(root_path, path))
                         for path in paths})

    with open(output_file_path, 'w') as out:
        for path in paths_uniq:
            # Assert that the file exists, since it was sourced, it
            # must surely also exist.
            assert os.path.isfile(path), "Internal error: '{}' does not exist".format(path)

            out.write("{}\n".format(path))


def parse_args():
    parser = argparse.ArgumentParser()

    parser.add_argument("kconfig_root")
    parser.add_argument("dotconfig")
    parser.add_argument("autoconf")
    parser.add_argument("sources")
    parser.add_argument("conf_fragments", nargs='+')

    return parser.parse_args()


def warn(msg):
    # Use a large fill() width to try to avoid linebreaks in the symbol
    # reference link. Add some extra newlines to set the message off from
    # surrounding text (this usually gets printed as part of spammy CMake
    # output).
    print("\nwarning: " + textwrap.fill(msg, 100) + "\n", file=sys.stderr)


def err(msg):
    sys.exit("\nerror: " + textwrap.fill(msg, 100) + "\n")


if __name__ == "__main__":
    main()
