find . -name "*.cpp" | xargs grep IDS_ | perl -ane ' foreach(@F) { if(/IDS_/) { s/(.*)(IDS_[a-zA-Z_0-9]+).*/\2/; print $_, "\n" } }' | sort | uniq 
#find . -name "*.cpp" | xargs grep IDS_ | perl -ne 's/(.*)(IDS_[a-zA-Z_0-9]+).+/\2/; print ' | sort | uniq  > strings.res

find . -name "*.cpp" | xargs grep IDS_ | perl -ane ' foreach(@F) { if(/IDS_/) { s/(.*)(IDS_[a-zA-Z_0-9]+).*/\2/; print "\"", $_, "\",\n" } }' | sort | uniq
#find . -name "*.cpp" | xargs grep IDS_ | perl -ne 's/(.*)(IDS_[a-zA-Z_0-9]+).+/\2/; chomp; print "\"$_\",\n" ' | sort | uniq  > strings_val.res


# then in vim open string.res and do: %s/$/\=printf(' = %-4d,', line('.')) to add ids, to enumerations
