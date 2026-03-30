proc main begin
read a
read b
temp0 = a > b
temp2 = !temp0
if temp2 goto Label1
temp1 = a + b
n = temp1
goto Label0
Label1:
temp3 = a - b
n = temp3
Label0:
print n
proc main end

