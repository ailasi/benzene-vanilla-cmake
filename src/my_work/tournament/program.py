"""Connects to a Hex program."""

import os
import subprocess
import sys
from random import randrange
from select import select


class Program:
    class CommandDenied(Exception):
        pass

    class Died(Exception):
        pass

    def __init__(self, color, command, log_name, verbose):
        command = command.replace("%SRAND", repr(randrange(0, 1000000)))

        self._command = command
        self._color = color
        self._verbose = verbose

        if self._verbose:
            print("Creating program:", command)

        p = subprocess.Popen(command, shell=True,
                             stdin=subprocess.PIPE,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE)

        self._stdin = p.stdin
        self._stdout = p.stdout
        self._stderr = p.stderr

        self._is_dead = 0
        self._log = open(log_name, "w")
        self._log.write("# " + self._command + "\n")

    def get_color(self):
        return self._color

    def get_command(self):
        return self._command

    def get_deny_reason(self):
        return self._denyReason

    def get_name(self):
        name = "?"
        try:
            name = self.send_command("name").strip()
            version = self.send_command("version").strip()
            name += " " + version
        except Program.CommandDenied:
            pass
        return name

    def get_result(self):
        try:
            ans = self.send_command("final_score")
            return ans.strip()
        except Program.CommandDenied:
            return "?"

    def get_time_remaining(self):
        try:
            ans = self.send_command("time_left")
            return ans.strip()
        except Program.CommandDenied:
            return "?"

    def is_dead(self):
        return self._is_dead

    def send_command(self, cmd):
        try:
            self._log.write(">" + cmd + "\n")
            if self._verbose:
                print(self._color + "< " + cmd)
            self._stdin.write((cmd + "\n").encode())
            self._stdin.flush()

            return self._get_answer()
        except IOError:
            self._program_died()

    def _get_answer(self):
        self._log_std_err()
        answer = ""
        done = 0
        number_lines = 0
        while not done:
            line = self._stdout.readline().decode()
            if line == "":
                self._program_died()
            self._log.write("<" + line)
            if self._verbose:
                sys.stdout.write(self._color + "> " + line)
            number_lines += 1
            done = (line == "\n")
            if not done:
                answer += line
        if answer[0] != '=':
            self._denyReason = str.strip(answer[2:])
            raise Program.CommandDenied
        if number_lines == 1:
            return str.strip(answer[1:])
        return answer[2:]

    def _log_std_err(self):
        files = select([self._stderr], [], [], 0)[0]
        for file in files:
            self._log.write(str(os.read(file.fileno(), 8192)))
        self._log.flush()

    def _program_died(self):
        self._is_dead = 1
        self._log_std_err()
        raise Program.Died
