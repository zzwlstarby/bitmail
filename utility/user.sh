#!/bin/bash

nick=$1 #"Harry QMail"
email=$2 #"751302230@qq.com"
org="bitmail.bitbaba.com"
ou="bitmail dept. of bitbaba.com corp."

openssl genrsa -aes256 -out userkey.pem 2048 

openssl req -new -key userkey.pem -subj "/CN=$nick/emailAddress=$email/O=$org/OU=$ou" -out usercsr.pem

cat <<heredoc >user.v3.ext
subjectKeyIdentifier=hash
authorityKeyIdentifier=keyid:always,issuer
basicConstraints=CA:false
keyUsage = digitalSignature, nonRepudiation, keyEncipherment, dataEncipherment, keyAgreement, keyCertSign, cRLSign
extendedKeyUsage=anyExtendedKeyUsage 
nsCertType = email
heredoc

echo subjectAltName=email:$email >> user.v3.ext
echo issuerAltName=email:$email  >> user.v3.ext

openssl x509 -req -in usercsr.pem -extfile user.v3.ext -CA cacert.pem -CAkey cakey.pem -days 3650 -CAcreateserial -out usercert.pem

openssl pkcs12 -export -aes256 -in usercert.pem -inkey userkey.pem -certfile cacert.pem -out user.p12

