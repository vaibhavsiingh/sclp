proc f1 begin
temp0 = b + c
d = temp0
temp1 = d + 2
stemp0 = temp1
goto Label0
Label0:
return stemp0
proc f1 end

proc g begin
stemp0 = b
goto Label1
Label1:
return stemp0
proc g end

proc main begin
a = 3
temp0 = f1(a, a)
a = temp0
print a
proc main end

