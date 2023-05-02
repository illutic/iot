#!/bin/sh

while getopts ":c:p:P:a" opt; do
  case $opt in
    c) CLIENT_SSID="$OPTARG"
    ;;
    p) CLIENT_PASSPHRASE="$OPTARG"
    ;;
    a) AP_SSID="$OPTARG"
    ;;
    P) AP_PASSPHRASE="$OPTARG"
    ;;
    \?) echo "Invalid option -$OPTARG" >&2
    exit 1
    ;;
  esac

  case $OPTARG in
    -*) echo "Option $opt needs a valid argument"
    exit 1
    ;;
  esac
done

sudo -Es
apt install hostapd

# Create this file with your settings for ssid=, country_code= and wpa_passphrase=. As channel= select the same channel wpa_supplicant with wlan0 will connect to your internet router.
#ATTENTION! This is a restriction from the hardware. hostapd will always set the channel to the same value than from the client connection, no matter what you set here. If you need different channels then you have to use an additional USB/WiFi dongle.
cat > /etc/hostapd/hostapd.conf <<EOF
driver=nl80211
ssid=$AP_SSID
country_code=GB
hw_mode=g
channel=1
auth_algs=1
wpa=2
wpa_passphrase=$AP_PASSPHRASE
wpa_key_mgmt=WPA-PSK
wpa_pairwise=TKIP
rsn_pairwise=CCMP
EOF

chmod 600 /etc/hostapd/hostapd.conf

#Create a service for the accesspoint with hostapd:
echo "
[Unit]
Description=accesspoint with hostapd (interface-specific version)
Wants=wpa_supplicant@%i.service

[Service]
ExecStartPre=/sbin/iw dev %i interface add ap@%i type __ap
ExecStart=/usr/sbin/hostapd -i ap@%i /etc/hostapd/hostapd.conf
ExecStopPost=-/sbin/iw dev ap@%i del

[Install]
WantedBy=sys-subsystem-net-devices-%i.device" > /etc/systemd/system/accesspoint@.service


sudo systemctl daemon-reload
sudo systemctl enable accesspoint@wlan0.service
rfkill unblock wlan

# setup wpa_supplicant for client connection
cat >/etc/wpa_supplicant/wpa_supplicant-wlan0.conf <<EOF
country=GB
ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev
update_config=1

network={
    ssid="$CLIENT_SSID"
    psk="$CLIENT_PASSPHRASE"
    key_mgmt=WPA-PSK
}
EOF

chmod 600 /etc/wpa_supplicant/wpa_supplicant-wlan0.conf

systemctl edit wpa_supplicant@wlan0.service

echo "
[Unit]
Description=accesspoint with hostapd (interface-specific version)
Wants=wpa_supplicant@%i.service

[Service]
ExecStartPre=/sbin/iw dev %i interface add ap@%i type __ap
ExecStart=/usr/sbin/hostapd -i ap@%i /etc/hostapd/hostapd.conf
ExecStopPost=-/sbin/iw dev ap@%i del

[Install]
WantedBy=sys-subsystem-net-devices-%i.device" > /etc/systemd/system/accesspoint.service

grep -qxF 'include "/lib/systemd/system/wpa_supplicant@.service"' /lib/systemd/system/wpa_supplicant@.service || echo 'include "/lib/systemd/system/wpa_supplicant@.service"' >> /lib/systemd/system/wpa_supplicant@.service