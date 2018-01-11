cfg () 
{ 
    build=$(gcc -dumpmachine)
    : ${builddir=build/$build}
    mkdir -p $builddir;
    ( set -x; cd $builddir;
    ../../configure \
          --prefix=/usr \
          --sysconfdir=/etc \
          --localstatedir=/var \
          --disable-{silent-rules,dependency-tracking} \
          --disable-rpath \
          --with-{libiconv,libintl,audiofile,esd,sdl}-prefix=/usr \
          --enable-debug \
          "$@"
    )
         grep -r '\-O[0-9]' $builddir -lI |xargs sed -i 's,-O[1-9],-ggdb -O0,'


}

