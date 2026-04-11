proc q begin
a = 1
temp0 = a * b
a = temp0
proc q end

proc p begin
b = 2
temp0 = b < d
temp2 = !temp0
if (temp2) goto Label1
temp1 = a + b
c = temp1
goto Label0
Label1:
Label0:
temp3 = c + d
x = temp3
proc p end

proc main begin
a = 5
b = 3
c = 7
d = 2
p()
temp0 = a + 2
a = temp0
temp1 = c + d
e = temp1
temp2 = a * b
d = temp2
temp3 = a + c
temp4 = temp3 + e
x = temp4
proc main end

