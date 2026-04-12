proc p begin
temp0 = 1 < 2
temp2 = !temp0
if (temp2) goto Label1
temp1 = a * b
a = temp1
goto Label0
Label1:
temp3 = 1 < 4
temp5 = !temp3
if (temp5) goto Label3
temp4 = a * b
c = temp4
c = a
goto Label2
Label3:
Label2:
Label0:
proc p end

proc main begin
temp0 = a * b
c = temp0
p()
p()
temp1 = a * b
a = temp1
proc main end

