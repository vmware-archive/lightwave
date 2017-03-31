host="localhost"
port=389

while getopts h:p: opt;
do
    case $opt in
        h)
            host=$OPTARG
            ;;
        p)
            port=$OPTARG
            ;;
    esac
done



