""" Plays a tournament between two Hex bots. """

import getopt
import sys

from tournament import IterativeTournament, RandomTournament


def print_usage():
    sys.stderr.write(
        "Usage: main.py [options]\n"
        "Options:\n"
        "  --dir      |-o: directory for storing results\n"
        "  --help     |-h: print help\n"
        "  --openings |-l: openings to use\n"
        "  --p1cmd    |-b: command of first program\n"
        "  --p1name   |  : name of first program\n"
        "  --p2cmd    |-w: command of second program\n"
        "  --p2name   |  : name of second program\n"
        "  --quiet    |-q: do not show log and board after each move\n"
        "  --rounds   |-r: number of rounds (default 1)\n"
        "  --size     |-s: board size (default 11)\n"
        "  --type     |-t: type of tournament ('iterative' or 'random')\n")


def main():
    rounds = 1
    board_size = 7
    verbose = False
    program1_name = ""
    program1_command = ""
    program2_name = ""
    program2_command = ""
    openings_file_path = ""
    results_directory = ""
    tournament_type = "iterative"

    try:
        options = "b:ho:s:w:ql:t:r:"
        long_options = ["p1cmd=", "p1name=", "p2cmd=", "p2name=",
                        "rounds=", "help", "dir=", "size=",
                        "quiet", "openings=", "type="]
        opts, args = getopt.getopt(sys.argv[1:], options, long_options)
    except getopt.GetoptError:
        print_usage()
        sys.exit(1)

    for option, value in opts:
        print("option = '" + option + "', value = '" + value + "'")
        if option in ("-b", "--p1cmd"):
            program1_command = value
        elif option in "--p1name":
            program1_name = value
        elif option in ("-w", "--p2cmd"):
            program2_command = value
        elif option in "--p2name":
            program2_name = value
        elif option in ("-r", "--rounds"):
            rounds = int(value)
        elif option in ("-h", "--help"):
            print_usage()
            sys.exit()
        elif option in ("-o", "--dir"):
            results_directory = value
        elif option in ("-s", "--size"):
            board_size = int(value)
        elif option in ("-q", "--quiet"):
            verbose = False
        elif option in ("-l", "--openings"):
            openings_file_path = value
        elif option in ("-t", "--type"):
            tournament_type = value

    if (program1_command == "" or program1_name == "" or
            program2_command == "" or program2_name == "" or
            openings_file_path == "" or results_directory == ""):
        print_usage()
        sys.exit(1)

    if tournament_type == "random":
        sys.stderr.write("Can't do random tournaments yet. :(")
        sys.exit(1)
    elif tournament_type == "iterative":
        IterativeTournament(program1_name, program1_command, program2_name, program2_command, board_size,
                            rounds, results_directory, openings_file_path, verbose).play_tournament()
    else:
        print("Unknown tournament type!\n")


main()
