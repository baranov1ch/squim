#!/usr/bin/env python

# Largely copied from google depot_tools git_cl.py.
# https://chromium.googlesource.com/chromium/tools/depot_tools.git/+/master/git_cl.py

import argparse
import logging
import os
import subprocess
import sys

def DieWithError(message):
  print >> sys.stderr, message
  sys.exit(1)

def RunCommand(args, error_ok=False, error_message=None, **kwargs):
  try:
    return subprocess.check_output(args, shell=False, **kwargs)
  except subprocess.CalledProcessError as e:
    logging.debug('Failed running %s', args)
    if not error_ok:
      DieWithError(
          'Command "%s" failed.\n%s' % (
            ' '.join(args), error_message or e.output or ''))
    return e.output

def RunGit(args, **kwargs):
  """Returns stdout."""
  return RunCommand(['git'] + args, **kwargs)

def GetRelativeRoot():
  return RunGit(['rev-parse', '--show-cdup']).strip()

def ShortBranchName(branch):
  """Convert a name like 'refs/heads/foo' to just 'foo'."""
  return branch.replace('refs/heads/', '')

def GetBranch():
  """Returns the short branch name, e.g. 'master'."""
  branchref = RunGit(['symbolic-ref', 'HEAD'],
                     stderr=subprocess2.VOID, error_ok=True).strip()
  return branchref

def BuildGitDiffCmd(diff_type, upstream_commit, args, extensions):
  """Generates a diff command."""
  # Generate diff for the current branch's changes.
  diff_cmd = ['diff', '--no-ext-diff', '--no-prefix', diff_type,
              upstream_commit, '--' ]

  if args:
    for arg in args:
      if os.path.isdir(arg):
        diff_cmd.extend(os.path.join(arg, '*' + ext) for ext in extensions)
      elif os.path.isfile(arg):
        diff_cmd.append(arg)
      else:
        DieWithError('Argument "%s" is not a file or a directory' % arg)
  else:
    diff_cmd.extend('*' + ext for ext in extensions)

  return diff_cmd

def Format(parser, args):
  """Runs auto-formatting tools (clang-format etc.) on the diff."""
  CLANG_EXTS = ['.cc', '.cpp', '.h', '.mm', '.proto']
  parser.add_argument('--upstream', nargs='?', default='master', type=str,
                      help='Second branch for merge-base')
  parser.add_argument('--dry-run', action='store_true',
                      help='Don\'t modify any file on disk.')
  parser.add_argument('--diff', action='store_true',
                      help='Print diff to stdout rather than modifying files.')
  opts = parser.parse_args(args)

  rel_base_path = GetRelativeRoot()
  if rel_base_path:
    os.chdir(rel_base_path)

  upstream_commit = RunGit(['merge-base', 'HEAD', opts.upstream])
  upstream_commit = upstream_commit.strip()

  if not upstream_commit:
    DieWithError('Could not find base commit for this branch. '
                 'Are you in detached state?')

  # Only list the names of modified files.
  diff_type = '--name-only'
  diff_cmd = BuildGitDiffCmd(diff_type, upstream_commit, args, CLANG_EXTS)
  diff_output = RunGit(diff_cmd)

  top_dir = os.path.normpath(
      RunGit(["rev-parse", "--show-toplevel"]).rstrip('\n'))

  # Locate the clang-format binary in the checkout
  clang_format_tool = os.path.join(
    top_dir, 'src/tools/clang/linux64/clang-format')
  if not os.path.isfile(clang_format_tool):
    DieWithError('clang_format binary not found. Have someone deleted it? '
                 'Why on Earth?')

  # diff_output is a list of files to send to clang-format.
  files = diff_output.splitlines()
  if files:
    cmd = [clang_format_tool]
    if not opts.dry_run and not opts.diff:
      cmd.append('-i')
    stdout = RunCommand(cmd + files, cwd=top_dir)
    if opts.diff:
      sys.stdout.write(stdout)

def main():
  parser = argparse.ArgumentParser()
  Format(parser, sys.argv[1:])

if __name__ == '__main__':
  main()
