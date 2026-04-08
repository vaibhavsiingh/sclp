proc main begin
n = 5
fact = 1
c = 1
Label0:
temp0 = c <= n
temp3 = !temp0
if (temp3) goto Label1
temp1 = fact * c
fact = temp1
temp2 = c + 1
c = temp2
goto Label0
Label1:
proc main end

