import hashlib

def h(str):
	hash = hashlib.sha256(str).digest()
	result = [
		(((x[0]*256)+x[1])*256+x[2])*256+x[3] for x in [hash[i:i+4] for i in range(0, 32, 4)]
	]
	return result

print(h(b"osdfi sdojrghyejyttukFGYJGFJdYJjh4%6y6j7*#45Gh*k8OnbtdgYHb$56H*Bh8^n e75gW46v^b7g54bh@%654HbiB8hjNTWgWv54^w%$5qq5$G^q546Q%6Qvg$%BghtHYDhbQ45G^dIOf894 DS34ds fsd %$$fdst%srdg&&^ggr34ggh35F#$G$g34ghffgfdfHFgDsfDf hg ghfd bwtGrt erTrEh1342Guhb89kuh"))