# PowerShell tab completion for tdf.py -- dot-sourced from export.ps1.
# Completes remaining pipeline commands (build flash monitor ...) and flags,
# mirroring tdf.bash / _tdf. Registers for `tdf`, `tdf.py`, and `tdf.cmd`.

$script:TdfCommands = @(
    'build', 'flash', 'monitor', 'clean', 'clean-device', 'create-project', 'help'
)
$script:TdfOptions = @(
    '-D', '--reconfigure', '--port', '--baud', '--no-color', '-y', '--yes', '--dir', '-h', '--help'
)

$tdfCompleter = {
    param($wordToComplete, $commandAst, $cursorPosition)

    # Tokens already on the line (skip element 0, the command name itself).
    $elements = @($commandAst.CommandElements | Select-Object -Skip 1 | ForEach-Object { "$_" })
    $prev = if ($elements.Count -ge 1) { $elements[-1] } else { '' }
    # When mid-word, the last element IS $wordToComplete -- look one back for "prev".
    if ($prev -eq $wordToComplete -and $elements.Count -ge 2) {
        $prev = $elements[-2]
    }

    switch -Regex ($prev) {
        '^--baud$' {
            return @('115200', '57600', '38400', '19200', '9600') |
                Where-Object { $_ -like "$wordToComplete*" } |
                ForEach-Object { [System.Management.Automation.CompletionResult]::new($_, $_, 'ParameterValue', $_) }
        }
        '^help$' {
            return $script:TdfCommands |
                Where-Object { $_ -like "$wordToComplete*" } |
                ForEach-Object { [System.Management.Automation.CompletionResult]::new($_, $_, 'ParameterValue', $_) }
        }
        '^(--port|--dir|-D)$' {
            # Fall back to PowerShell's default path completion (return nothing).
            return @()
        }
    }

    if ($wordToComplete -like '-*') {
        return $script:TdfOptions |
            Where-Object { $_ -like "$wordToComplete*" } |
            ForEach-Object { [System.Management.Automation.CompletionResult]::new($_, $_, 'ParameterName', $_) }
    }

    # Offer only pipeline commands not already present on the line.
    $remaining = $script:TdfCommands | Where-Object { $elements -notcontains $_ }
    return $remaining |
        Where-Object { $_ -like "$wordToComplete*" } |
        ForEach-Object { [System.Management.Automation.CompletionResult]::new($_, $_, 'Command', $_) }
}

Register-ArgumentCompleter -CommandName 'tdf', 'tdf.py', 'tdf.cmd' -Native -ScriptBlock $tdfCompleter
