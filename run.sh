make
if [ $? -ne 0 ] ; then
    exit
fi
command=""
for input; do
    command="${command} ${input}"
done
${command}
