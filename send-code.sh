# # Usage example:
# $ export BEARER="*****************"
# $ export USER="*****************"
# $ export DEVICE="*****************"
# $ export PARAM="action"
# $ export CODE="11767553"
# $ export PULSELENGHT="395"
# $ export REPEATTRANSMIT="5"
# $ ./send-code.sh 

for key in "BEARER" "USER" "DEVICE" "PARAM" "CODE" "PROTOCOL" "PULSELENGHT" "REPEATTRANSMIT"; do
    
    if [ "${!key}" == "" ];then
        echo "You should set \$$key."
        exit 1
    fi
    
done

curl -k \
-H "Content-Type: application/json;charset=UTF-8" \
-H "Authorization: Bearer $BEARER" \
-H "Accept: application/json, text/plain, */*" \
-X POST \
-d "{\"code\":$CODE,\"protocol\":$PROTOCOL,\"pulseLength\":$PULSELENGHT,\"repeatTransmit\":$REPEATTRANSMIT}" \
https://backend.thinger.io/v3/users/$USER/devices/$DEVICE/resources/$PARAM
