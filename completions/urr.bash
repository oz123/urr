# bash completion for urr(1)

_urr_extract_host() {
    local first second
    read -r first second <<< "$1"
    # If first field looks like a MAC address, hostname is second (ethers format)
    # Otherwise hostname is first (hostname mac format)
    if [[ "${first}" =~ ^([0-9a-fA-F]{2}[:\-]){5}[0-9a-fA-F]{2}$ ]]; then
        echo "${second}"
    else
        echo "${first}"
    fi
}

_urr_hosts() {
    local hosts_file="${1:-${HOME}/.config/urr/hosts}"

    if [[ -r "${hosts_file}" ]]; then
        while IFS= read -r line; do
            [[ "${line}" =~ ^[[:space:]]*# ]] && continue
            [[ -z "${line}" ]] && continue
            _urr_extract_host "${line}"
        done < "${hosts_file}"
    fi

    if [[ -r /etc/ethers ]]; then
        while IFS= read -r line; do
            [[ "${line}" =~ ^[[:space:]]*# ]] && continue
            [[ -z "${line}" ]] && continue
            _urr_extract_host "${line}"
        done < /etc/ethers
    fi
}

_urr() {
    local cur prev words cword
    _init_completion || return

    local opts="-v --version -h --help -f --file"
    local custom_file=""

    # Find -f/--file value already on the command line
    local i
    for (( i=1; i < cword; i++ )); do
        if [[ "${words[i]}" == "-f" || "${words[i]}" == "--file" ]]; then
            custom_file="${words[i+1]}"
            break
        fi
    done

    case "${prev}" in
        -f|--file)
            # Complete filenames
            _filedir
            return
            ;;
    esac

    if [[ "${cur}" == -* ]]; then
        COMPREPLY=( $(compgen -W "${opts}" -- "${cur}") )
    else
        # Complete hostnames from the hosts file
        local hosts
        mapfile -t hosts < <(_urr_hosts "${custom_file}")
        COMPREPLY=( $(compgen -W "${hosts[*]}" -- "${cur}") )
    fi
}

complete -F _urr urr
