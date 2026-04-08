proc main begin
a = 2
temp0 = a > 1
temp1 = !temp0
if (temp1) goto Label1
a = 2
goto Label0
Label1:
temp2 = a < 4
temp3 = !temp2
if (temp3) goto Label3
a = 4
goto Label2
Label3:
temp4 = a > 3
temp5 = !temp4
if (temp5) goto Label5
a = 5
goto Label4
Label5:
a = 6
Label4:
Label2:
Label0:
proc main end

