# clumsy

__clumsy makes your network condition on Windows significantly worse, but in a managed and interactive manner.__

Leveraging the awesome [WinDivert](http://reqrypt.org/windivert.html), clumsy stops living network packets and capture them, lag/drop/tamper/.. the packets on demand, then send them away. Whether you want to track down weird bugs related to broken network, or evaluate your application on poor connections, clumsy will come in handy:

* No installation.
* No need for proxy setup or code change in your application.
* System wide network capturing means it works on any application.
* Works even if you're offline (ie, connecting from localhost to localhost).
* Your application keeps running, while clumsy can start and stop anytime.
* Interactive control how bad the network can be, with enough visual feedback to tell you what's going on.

See [this page](http://jagt.github.io/clumsy) for more info and build instructions.


## Details

Simulate network latency, delay, packet loss with clumsy on Windows 7/8/10:

![](clumsy-demo.gif)

### Application filter

This fork can limit impairments to a specific application. The normal packet
filter still controls which packets WinDivert captures. When "Limit to
application" is enabled, clumsy only applies lag/drop/throttle/etc. to packets
attributed to the selected executable name or full path. Other matching packets
are passed through unchanged.

Example:

- Packet filter: `outbound and (tcp or udp)`
- Application: `Game.exe`

Notes:

- Multiple applications can be separated with commas or semicolons.
- Process-name matching is case-insensitive.
- Browse can fill in a full executable path and switches matching to full path.
- Child-process matching is always enabled, which helps portable launchers such
  as `SpeedtestPortable.exe` target the real network process they launch.
- When the application filter is enabled, clumsy narrows packet capture to
  known target flows so non-target applications are not routed through the
  impairment loop just because the packet preset is broad.
- The app does not need to be running before clumsy starts.
- Unknown/unattributed packets pass through unchanged.
- Loopback attribution has the same caveats as clumsy's existing loopback support.


## License

MIT
