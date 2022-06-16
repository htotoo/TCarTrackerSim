<?php
$posted = file_get_contents('php://input');
file_put_contents("postbe.txt", $posted); //for debug

if (strpos($posted, "+CMGL:")>1)
{
    //found a new sms, send via email
    mail('WRITEIT@DOMAIN:COM', 'SMS from CAR', $posted);
}

?>