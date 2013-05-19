all:
	g++ MyOnlineCoder.cpp -o coder -lm -g
	g++ MyOnlineDecoder.cpp -o decoder -lm -g
clean:
	rm coder decoder coded_timestamps decoded_timestamps statistics.txt
