filename$ = "./test.wav"
Read from file... ../A/'fileName$'
name$ = fileName$ - ".wav"
select Sound 'name$'
 To Pitch (ac)... 0.0 50.0 15 off 0.1 0.60 0.01 0.35 0.14 500.0

	numFrame=Get number of frames
	    for i to numFrame
		time=Get time from frame number... i
		value=Get value in frame... i Hertz
		if value = undefined
			value=0
		endif
		path$="./data/PITCH/"+name$+"_pitch.txt"
		fileappend 'path$' 'time' 'value' 'newline$'
	    endfor
		select Pitch 'name$'
		Remove
	endfor
