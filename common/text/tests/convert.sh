odin "%bitmap2text >/tmp/b2t"
b2t=/tmp/b2t

for file in /no\ name/Mvc-*.jpg ; do
	basename=$(echo $(basename "$file") | sed "s/\(.*\)\..*/\1/")
	echo $file $basename
	for size in "40 15" "80 30 " "160 60" "320 120" ; do
		$b2t "$file" $size 0.1 0.9 0.75 >"$basename ($size).html"
	done
done

