# bash completion for tdf.py -- sourced from export.sh.
# Completes remaining pipeline commands (build flash monitor ...) and flags.

_tdf() {
    local cur prev
    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"

    local cmds="build flash monitor clean clean-device create-project help"
    local opts="-D --reconfigure --port --baud --no-color -y --yes --dir -h --help"

    case "$prev" in
        --port)
            COMPREPLY=( $(compgen -f -- "$cur") )
            return
            ;;
        --baud)
            COMPREPLY=( $(compgen -W "115200 57600 38400 19200 9600" -- "$cur") )
            return
            ;;
        --dir)
            COMPREPLY=( $(compgen -d -- "$cur") )
            return
            ;;
        help)
            COMPREPLY=( $(compgen -W "build flash monitor clean clean-device create-project help" -- "$cur") )
            return
            ;;
        -D)
            return
            ;;
    esac

    if [[ "$cur" == -* ]]; then
        COMPREPLY=( $(compgen -W "$opts" -- "$cur") )
        return
    fi

    local remaining="" c w seen
    for c in $cmds; do
        seen=0
        for w in "${COMP_WORDS[@]}"; do
            if [[ "$w" == "$c" ]]; then
                seen=1
                break
            fi
        done
        if [[ $seen -eq 0 ]]; then
            remaining="$remaining $c"
        fi
    done
    COMPREPLY=( $(compgen -W "$remaining" -- "$cur") )
}

complete -F _tdf tdf.py tdf
