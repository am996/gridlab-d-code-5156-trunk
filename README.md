# gridlab-d-code-5156-trunk

Περιεχόμενα:
Η παρούσα καταχώρηση περιέχει τον φάκελο gridlab-d-code-5156-trunk που αφορά τον client και εντός αυτού τον φάκελο energy που αφορά τον server.

Server:
Για την εγκατάσταση και εκτέλεση των αρχείων του server είναι απαραίτητη η ύπαρξη των axis2c και staff.

Οδηγίες εγκατάστασης των services:<br>
1. Αντιγραφή και επικόλληση του φακέλου energy στην διεύθυνση /usr/local/staff/bin<br>
2. Σε κάθε session δίνουμε στο terminal<br>
export AXIS2C_HOME=/usr/local/axis2c<br>
export STAFF_HOME=/usr/local/staff<br>
export PATH=$PATH:/usr/local/staff/bin<br>
είτε τα προσθέτουμε ως environmental variables<br>
3. Εκτελούμε make ή sudo make και μετά sudo -E make install στις παρακάτω διευθύνσεις<br>
/usr/local/staff/bin/1<br>
/usr/local/staff/bin/2<br>
/usr/local/staff/bin/3<br>
/usr/local/staff/bin/4<br>
/usr/local/staff/bin/5<br>
4. Στην διεύθυνση /usr/local/axis2c/bin εκτελούμε ./axis2_http_server ώστε να ξεκινήσει η λειτουργία του server<br>

Client:<br>
1. Αντιγράφουμε τον φάκελο gridlab-d-code-5156-trunk στον υποφάκελο code της εγκατάστασης του GridLAB-D<br>
2. Μέσω του cmd καλούμε το αντίστοιχο test file με την εντολή <br>
  gridlabd test*.glm
