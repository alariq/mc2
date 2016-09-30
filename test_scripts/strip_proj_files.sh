cat $1 | grep ".cpp" | perl -pe 's/.+\"([\w\\]+\.cpp)\".+/\1/g; s/\\/\//g; ' 

