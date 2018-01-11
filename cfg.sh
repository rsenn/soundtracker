cfg () 
{ 
    build=$(gcc -dumpmachine)
    builddir=build/$build
    mkdir -p $builddir;
    ( set -x; cd $builddir;
    ../../configure \
          --prefix=/usr \
          --sysconfdir=/etc \
          --program-suffix="-gtk2" \
          --localstatedir=/var \
          --disable-{silent-rules,dependency-tracking} \
          --disable-rpath \
          --disable-{audiofiletest,esdtest,sdltest} \
          --with-{libiconv,libintl,audiofile,esd,sdl}-prefix=/usr \
          "$@"
    )

}

