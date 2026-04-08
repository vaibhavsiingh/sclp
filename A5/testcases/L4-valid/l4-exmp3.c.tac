proc main begin
a = 5
i = 0
print "ldoa"
Label0:
temp0 = i < a
temp2 = !temp0
if (temp2) goto Label1
temp1 = i + 1
i = temp1
print i
goto Label0
Label1:
proc main end

