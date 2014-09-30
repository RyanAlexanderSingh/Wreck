import sys
import os
import dircache
import string

proto_dir = 'src/examples/example_physics/'
target_dir = 'src/examples/'

def make_example(projname):
  try:
    os.mkdir(target_dir + projname)
  except:
    pass
  for fname in dircache.listdir(proto_dir):
    if 'prototype' in fname:
      dest_name = string.replace(fname, 'example_physics', projname)
    else:
      dest_name = fname
    print(target_dir + projname + '/' + dest_name)
    try:
      test = open(target_dir + projname + '/' + dest_name, "rb")
      test.close()
    except:
      out = open(target_dir + projname + '/' + dest_name, "wb")
      for line in open(proto_dir + fname):
        line = string.replace(line, 'example_physics', projname)
        out.write(line)


num_args = len(sys.argv)

if num_args == 1 or "--help" in sys.argv:
  print("\nmakes or updates a new octet project in src/examples")
  print("\nusage: packaging\make_example.py projectname")
  exit()

for i in range(1,num_args):
  arg = sys.argv[i];
  if arg[0] != '-':
    make_example(arg)
  else:
    print("unrecognised option " + arg)
    exit()
