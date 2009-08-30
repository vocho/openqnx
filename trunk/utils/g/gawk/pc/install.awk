# install.awk 
# awk script to handle "make install". Goal is to eliminate need for
# extra utilities (such as sh, mkdir, and cp). This is a hack.

function mkinstalldirs(dir,  i, ii, j, jj, s, comp, mkdir)
{
  gsub("/", "\\", dir); ii = split(dir, s, " ")
  print "@echo off" > install_bat
  print "@echo off" > install_cmd
  for (i = 1; i <= ii; i++) {
    jj = split(s[i], comp, "\\"); dir = comp[1];
    for (j = 1; j <= jj; dir=dir "\\" comp[++j]) {
      if (substr(dir, length(dir)) == ":" || mkdir[dir]) continue;
      printf("if not exist %s\\*.* mkdir %s\n", dir, dir) > install_bat
      printf("if not exist %s\\* mkdir %s\n", dir, dir) > install_cmd
      mkdir[dir] = 1
    }
  }
  close(install_bat); close(install_cmd)
  system(install)
}

function cp(s,  j, n, comp)
{
  gsub("/", "\\", s); n = split(s, comp, " ");
  print "@echo off" > install_bat
  print "@echo off" > install_cmd
  for (j = 1; j < n; j++) {
    printf("copy %s %s\n", comp[j], comp[n]) > install_cmd
    if (substr(comp[j], length(comp[j]), 1) == "*")
      comp[j] = comp[j] ".*"
    printf("copy %s %s\n", comp[j], comp[n]) > install_bat
  }
  close(install_bat); close(install_cmd)
  system(install)
}
    
BEGIN{
install = "installg"
install_bat = install ".bat"; install_cmd = install ".cmd"
igawk_cmd = prefix "/bin/igawk.cmd"
igawk_bat = prefix "/bin/igawk.bat"
igawk = "pc/awklib/igawk"

# Make the bin directory
mkinstalldirs(prefix "/bin");

# Create igawk.cmd for OS/2
printf("extproc sh %s/bin/igawk.cmd\nshift\n", prefix) > igawk_cmd
while (getline < igawk) print $0 > igawk_cmd

# Create igawk.bat for DOS
printf("@sh %s/bin/igawk %%1 %%2 %%3 %%4 %%5 %%6 %%7 %%8 %%9", prefix) > igawk_bat

# Do common
cp(igawk " *awk.exe " prefix "/bin")
mkinstalldirs(prefix "/lib/awk " prefix "/man/man1 " prefix "/info")
cp("awklib/eg/lib/* pc/awklib/igawk.awk " prefix "/lib/awk");
cp("doc/*.1 " prefix "/man/man1");
cp("doc/gawk.info " prefix "/info");
}
