#!/bin/bash
for PO_FILE in locale/*/LC_MESSAGES/*.po
do
    MO_FILE="${PO_FILE/.po/.mo}"
    msgfmt -o "$MO_FILE" "$PO_FILE"
done