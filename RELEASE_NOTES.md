# clumsy 0.4

clumsy 0.4 adds native application-level filtering for Windows.

## Highlights

- New "Limit to application" controls in the Filtering panel.
- Match target apps by process name, such as `Speedtest.exe`, or by full executable path.
- Browse button for selecting an executable and switching to full-path mode.
- Multiple application targets separated with commas or semicolons.
- Child-process matching for launchers and portable apps.
- WinDivert FLOW tracking plus IP Helper seeding for existing and new TCP/UDP flows.
- Target-aware packet capture so broad packet filters do not unnecessarily route unrelated apps through the impairment loop.
- Live status counters for target PIDs, target flows, UDP endpoints, affected packets, passed packets, and unknown packets.

## Download

Download `clumsy-0.4-win64.zip`, extract it, and run `clumsy.exe` as Administrator.

The zip contains:

- `clumsy.exe`
- `WinDivert.dll`
- `WinDivert64.sys`
- `iup.dll`
- `config.txt`
- `LICENSE.txt`
- `README.md`

## Notes

- Unknown or unattributed packets pass through unchanged.
- The application filter does not use `processId` in a NETWORK-layer WinDivert filter.
- Portable launchers can be targeted directly when child-process matching finds the real network-owning child process.
