#!/usr/bin/env sh

printError() {
    printf "The program <%s> is required and could not be found in %s\n" "$1" "\$PATH"
}

if [ -z "$1" ]; then
    printf "Usage: %s <keyword-in-package-name-or-desc>\n" "$0"
    exit 1
fi

if ! command -v jq  >/dev/null 2>&1; then
    printError 'jq'
    exit 2
fi

if ! command -v curl  >/dev/null 2>&1; then
    printError 'curl'
    exit 2
fi

if ! command -v column  >/dev/null 2>&1; then
    printError 'column'
    exit 2
fi

url="https://aur.archlinux.org/rpc/?v=5&type=search&arg=$1"
curl --silent "$url" \
    | jq -r '(.results|.[]|[.Name,.Description,.Version,"https://aur.archlinux.org/" + .Name]) | @tsv' \
    | column -t -N Name,Description,Version,Clone -W Description -s '	'

cmd_exit="$?"

if [ $cmd_exit -ne 0 ]; then
    exit $cmd_exit
fi

