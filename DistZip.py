# DistZipBuilder: tool to generate zip files using SCons
#
# Copyright (C) 2005, 2006  Matthew A. Nicholson
# Copyright (C) 2006  John Pye
#
# This file is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License version 2.1 as published by the Free Software Foundation.
#
# This file is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

import os,sys
from SCons.Script import *
import SCons.Builder

def DistZipEmitter(target,source,env):

   source,origsource = [], source

   excludeexts = env.Dictionary().get('DISTZIP_EXCLUDEEXTS',[])
   excludedirs = env.Dictionary().get('DISTZIP_EXCLUDEDIRS',[])

   # assume the sources are directories... need to check that
   for item in origsource:
      if os.path.isdir(str(item)):
         for root, dirs, files in os.walk(str(item)):

            # don't make directory dependences as that triggers full build
            # of that directory
            if root in source:
               #print "Removing directory %s" % root
               source.remove(root)

            # loop through files in a directory
            for name in files:
               ext = os.path.splitext(name)
               if not ext[0] in excludeexts and not ext[1] in excludeexts:
                  relpath = os.path.join(root,name)
                  source.append(relpath)
            for d in excludedirs:
               if d in dirs:
                  dirs.remove(d)  # don't visit CVS directories etc
      else:
         ext = os.path.splitext(str(item))
         if not ext[1] in excludeexts:
            source.append(str(item))

   return target, source

def DistZipString(target, source, env):
   """
   This is what gets printed on the console. We'll strip out the list or source
   files, since it tends to get very long. If you want to see the contents, the
   easiest way is to uncomment the line 'Adding to ZIP file' below.
   """
   return 'DistZip(%s,...)' % str(target[0])

def DistZip(target, source, env):
   """zip archive builder"""

   import zipfile

   env_dict = env.Dictionary()

   # split the target directory, filename, and stuffix
   base_name = str(target[0]).split('.zip')[0]
   (target_dir, dir_name) = os.path.split(base_name)

   # create the target directory if it does not exist
   if target_dir and not os.path.exists(target_dir):
      os.makedirs(target_dir)

   # open our zip file for writing
   sys.stderr.write("DistZip: Writing " + str(target[0]))
   the_zip = zipfile.ZipFile(str(target[0]), "w")

   # write sources to our zip file
   for item in source:
      item = str(item)
      sys.stderr.write(".")
      #print "Adding to ZIP file: %s/%s" % (dir_name,item)
      the_zip.write(item, '%s/%s' % (dir_name,item), zipfile.ZIP_DEFLATED)

   # all done
   sys.stderr.write("\n") #print "Closing ZIP file"
   the_zip.close()

def DistZipSuffix(env, sources):
    """zip archive suffix generator"""
    return ".zip"

def generate(env):
   """
   Add builders and construction variables for the DistZip builder.
   """

   env.Append(BUILDERS = {
      'DistZip': env.Builder(
         action = SCons.Action.Action(DistZip, DistZipString),
         suffix = DistZipSuffix,
         emitter = DistZipEmitter,
         target_factory = env.fs.Entry,
      ),
   })


def exists(env):
   """
   Make sure this tool exists.
   """
   try:
      import os
      import zipfile
   except ImportError:
      return False
   else:
      return True
