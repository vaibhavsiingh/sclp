proc main begin
test = 0
loop = 10
Label2:
temp0 = loop > 1
temp4 = !temp0
if (temp4) goto Label3
temp1 = loop - 1
loop = temp1
temp2 = loop == 0
temp3 = !temp2
if (temp3) goto Label1
print loop
goto Label0
Label1:
print test
Label0:
goto Label2
Label3:
Label4:
temp5 = loop + 1
loop = temp5
temp6 = loop < 10
if (temp6) goto Label4
print loop
proc main end

