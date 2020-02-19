# Build system copied/modified from
# http://www.scons.org/cgi-bin/wiki/AdvancedBuildExample
# Original by zedshaw@zedshaw.com
import sys
import os
import fnmatch

def SelectBuildDir(build_dir, platform=None):

    # if no platform is specified, then default to sys.platform
    if not(platform):
        platform = sys.platform

    print("Looking for build directory for platform '%s'" % platform)

    # setup where we start looking at first
    test_dir = build_dir + os.sep + platform
    default_dir = build_dir + os.sep + 'default'


    # we look for a directory named exactly after the
    # platform so that very specific builds can be done
    if os.path.exists(test_dir):
        # make sure it is a directory
        target_dir = test_dir
    else:
        print("Exact match not found, finding closest guess")

        # looks like there isn't an exact match
        # find the closest matching directory
        dirs = os.listdir(build_dir)
        found_match = 0
        for dir in dirs:
            if platform.find(dir) != -1:
                 # found a match (hopefully the right one)
                 target_dir = build_dir + os.sep + dir
                 found_match = 1
                 break
        if not(found_match):
            print("No match found, looking for 'default' directory")
            # looks like this platform isn't available
            # try the default target
            if os.path.exists(default_dir):
                target_dir = default_dir
            else:
                # bad, nothing is available, tell the user
                print("No build directories found for your platform '%s'" % platform)
                return None

    print("Found directory %s, will build there" % target_dir)
    return target_dir


def RequireFiles(files, found_files, search_path):
    i = 0
    for file in found_files:
        if not(file):
            print("ERROR:  Could not find the %s file in:" % files[i])
            print("ERROR:  \t%s" % search_path)
            print("ERROR:  Edit the build_config.py file and add")
            print("ERROR:  the location of this file to the appropriate variable.")

            sys.exit(1)

        else:
            i = i + 1

# Like glob, but gets recursively all the files under the specified directory
def DirGlob(dir, match):
    match_list = []
    for root, dirs, files in os.walk(dir):
        for matching_file in fnmatch.filter(files, match):
            match_list.append(root + os.sep + matching_file)

    return match_list
