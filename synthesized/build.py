#!/usr/bin/python3

import pathlib
import glob
import argparse
import os
import subprocess

SYNTHESIZED_DIR = pathlib.Path(__file__).parent.absolute()

BOILERPLATE_DIR = f"{SYNTHESIZED_DIR}/boilerplate"
SYNTHESIZED_BUILD = f"{SYNTHESIZED_DIR}/build"

SYNTHESIZED_CODE = f"{SYNTHESIZED_BUILD}/synthesized"
SYNTHESIZED_BUNDLE = f"{SYNTHESIZED_BUILD}/bundle"

subprocess.call([ "mkdir", "-p", SYNTHESIZED_CODE ], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
subprocess.call([ "mkdir", "-p", SYNTHESIZED_BUNDLE ], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

def build_impl(boilerplate, impl):
  complete_impl = ""
  
  with open(f"{BOILERPLATE_DIR}/{boilerplate}.c", 'r') as f:
    complete_impl += f.read()

  complete_impl += "\n"

  with open(impl, 'r') as f:
    complete_impl += f.read()

  complete_impl += "\n"
  
  with open(f"{SYNTHESIZED_CODE}/nf.c", 'w') as f:
    f.write(complete_impl)

def get_original_nf_srcs(nf):
  original_nf_files = []

  if not nf:
    return original_nf_files

  original_nf_files += glob.glob(f"{nf}/*.c")
  original_nf_files += glob.glob(f"{nf}/*.h")
  original_nf_files += glob.glob(f"{nf}/*.py")
  original_nf_files += glob.glob(f"{nf}/*.ml")

  return original_nf_files

def build_makefile(extra_vars_makefile, nf, srcs):
  MAKEFILE = ""
  for f in srcs:
    if len(MAKEFILE) == 0:
      MAKEFILE += f"ORIGINAL_NF_FILES_ABS_PATH := {os.path.abspath(f)}\n"
      MAKEFILE += f"ORIGINAL_NF_FILES := {os.path.basename(f)}\n\n"
    else:
      MAKEFILE += f"ORIGINAL_NF_FILES_ABS_PATH += {os.path.abspath(f)}\n"
      MAKEFILE += f"ORIGINAL_NF_FILES += {os.path.basename(f)}\n\n"
  
  MAKEFILE += f"SYNTHESIZED_FILE := {SYNTHESIZED_DIR}/build/synthesized/nf.c\n"
  MAKEFILE += f"\ninclude {os.path.abspath(extra_vars_makefile)}\n"

  if not nf:
    MAKEFILE += f"include $(abspath $(dir $(lastword $(MAKEFILE_LIST))))/../Makefile\n"
  else:
    MAKEFILE += f"include {nf}/Makefile\n"

  makefile = open(f"{SYNTHESIZED_BUNDLE}/Makefile.nf", mode='w')
  makefile.write(MAKEFILE)
  makefile.close()

def build():
  subprocess.call([ "make", "-f", "Makefile.nf" ], cwd=SYNTHESIZED_BUNDLE)
  subprocess.call([ "cp", f"{SYNTHESIZED_BUNDLE}/build/app/*", f"{SYNTHESIZED_BUILD}/app" ],
    stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

BOILERPLATE_CHOICE_BMV2 = "bmv2_ss_grpc_controller"
BOILERPLATE_CHOICE_CALL_PATH_HITTER = "call_path_hitter"
BOILERPLATE_CHOICE_LOCKS = "locks"
BOILERPLATE_CHOICE_SQ = "sequential"
BOILERPLATE_CHOICE_SN = "shared-nothing"
BOILERPLATE_CHOICE_TM = "tm"

if __name__ == "__main__":
  parser = argparse.ArgumentParser(description='Bundle synthesized NF with existing Vigor NF.')
  
  parser.add_argument('impl', type=str, help='path to the file containing nf_init and nf_process implementations')
  parser.add_argument('boilerplate', 													\
    help='name of the boilerplate file', 											\
    choices=[
      BOILERPLATE_CHOICE_BMV2,
      BOILERPLATE_CHOICE_CALL_PATH_HITTER,
      BOILERPLATE_CHOICE_LOCKS,
      BOILERPLATE_CHOICE_SQ,
      BOILERPLATE_CHOICE_SN,
      BOILERPLATE_CHOICE_TM,
    ])
  parser.add_argument('extra_vars_makefile', type=str, help='path to the Makefile containing the extra vars')
  parser.add_argument('--nf', type=str, help='path to the original NF')

  args = parser.parse_args()

  if args.nf:
    args.nf = os.path.abspath(args.nf)

  build_impl(args.boilerplate, args.impl)
  srcs = get_original_nf_srcs(args.nf)
  build_makefile(args.extra_vars_makefile, args.nf, srcs)
  build()
