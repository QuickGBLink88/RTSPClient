# RTSPClient
an rtsp client, with rtp packet receiving, rtcp handing, video decoding, audio decoding, and playing functions.
Currently it suppports:
1. RTP over TCP(udp transmission is not implemented).
2. send heartbeat message(VIA OPTIONS) to server.
3. support video stream format: MPEG4, H264, H265.
4. support audio stream format: AMR, AAC, MP3.
5. support Authorization, that means it can login on server which needs username and password.
6. receive video and audio RTCP packet and get npt time info, the player can use both info to keep video-audio synchronize.
7. support video and audio decoding and playing, and decoding video/audio uses ffmpeg.

the client player is written in C++, with VS2008.
