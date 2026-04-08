proc main begin
a1 = 1
a2 = 1
a = 1
b = 1
c = 1
d = 1
Label13:
temp0 = a1 <= 100
temp22 = !temp0
if (temp22) goto Label14
temp1 = a + 1
a = temp1
temp2 = 2 >= 3
temp4 = !temp2
if (temp4) goto Label1
temp3 = a + 1
a = temp3
goto Label0
Label1:
Label0:
temp5 = 3 >= 2
temp7 = !temp5
if (temp7) goto Label3
temp6 = a + 1
a = temp6
goto Label2
Label3:
Label2:
temp8 = 2 <= 3
temp10 = !temp8
if (temp10) goto Label5
temp9 = a + 1
a = temp9
goto Label4
Label5:
Label4:
temp11 = 2 != 3
temp13 = !temp11
if (temp13) goto Label7
temp12 = a + 1
a = temp12
goto Label6
Label7:
Label6:
temp14 = 2 == 3
temp16 = !temp14
if (temp16) goto Label9
temp15 = a + 1
a = temp15
goto Label8
Label9:
Label8:
temp17 = 4 >= 3
temp19 = !temp17
if (temp19) goto Label11
temp18 = a + 1
a = temp18
goto Label10
Label11:
Label10:
Label12:
temp20 = a1 + 1
a1 = temp20
temp21 = a1 <= 200
if (temp21) goto Label12
print a1
goto Label13
Label14:
print a
proc main end

