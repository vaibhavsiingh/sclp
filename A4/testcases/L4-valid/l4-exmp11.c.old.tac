proc main begin
x = 14
y = 36
a = x
b = y
Label0:
temp0 = b != 0
temp3 = !temp0
if (temp3) goto Label1
t = b
temp1 = a / b
temp2 = temp1 * b
b = temp2
a = t
goto Label0
Label1:
gcd = a
temp4 = x * y
temp5 = temp4 / gcd
lcm = temp5
proc main end

