# FANS-MultiUAV-Hardware
Hardware implementation of Flying Ad-Hoc Network Simulator

For reference:

UAV1 -> UAV4 -> UAV3

UAV1: Sender Node
Server - Local, Client - Remote
LocalIP - 10.1.1.1
LocalMac - e8:4e:06:32:28:dd
RemoteIP - 10.1.1.4
RemoteMac - e8:4e:06:32:17:a6


UAV4: Forwarder Node
Server1 - Remote_1, Client1 - Local
Server2 - Local, Client2 - Remote_2
LocalIP - 10.1.1.4
LocalMac - e8:4e:06:32:17:a6
RemoteIP_1 - 10.1.1.1
RemoteMac_1 - e8:4e:06:32:28:dd
RemoteIP_2 - 10.1.1.3
RemoteMac_2 - e8:4e:06:32:

UAV3: Receiver Node
Server - Remote, Client - Local
LocalIP - 10.1.1.3
LocalMac - e8:4e:06:32:
RemoteIP - 10.1.1.4
RemoteMac - e8:4e:06:32:17:a6
