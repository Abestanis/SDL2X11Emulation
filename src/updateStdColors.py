def f(x):
	lines = x.split('\n')
	r = ''
	for l in lines:
		parts = l.split()
		r += '{"' + " ".join(parts[3:]) + '", 0x'
		for i in range(3):
			r += hex(int(parts[i]))[2:].upper().zfill(2)
		r += 'FF},\n'
	print(r)