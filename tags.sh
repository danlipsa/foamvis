# Builds the tags database for foamvis
find . -maxdepth 1 \
    -name '*.cpp' -or -name '*.h' -or -name '*.frag' -or -name '*.qrc'\
    -name '*.vert' -or -name '*.frag.in' -or -name '*.ui' -or -name '*.pro'\
    -name '*.y' -or -name '*.l' | \
    grep -v ui_ | grep -v _lex. | grep -v _yacc. | \
    grep -v 'TensorDisplay.frag$' | grep -v 'VectorDisplay.frag$' | \
    etags -
