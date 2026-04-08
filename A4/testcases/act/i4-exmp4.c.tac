proc main begin
read n
temp0 = n >= 5
temp1 = n <= 10
temp2 = temp0 && temp1
temp6 = !temp2
if (temp6) goto Label3
Label0:
temp3 = n > 0
temp5 = !temp3
if (temp5) goto Label1
print n
print " "
temp4 = n - 1
n = temp4
goto Label0
Label1:
goto Label2
Label3:
temp7 = n <= 1024
temp8 = -1024
temp9 = n >= temp8
temp10 = temp7 && temp9
temp13 = !temp10
if (temp13) goto Label6
Label4:
print n
print "/"
temp11 = n / 2
n = temp11
temp12 = n != 0
if (temp12) goto Label4
goto Label5
Label6:
print "Your number is:"
print n
Label5:
Label2:
proc main end

