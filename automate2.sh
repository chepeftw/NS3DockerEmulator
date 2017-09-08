#!/bin/bash

NUMBER=25
TIME=50

python main.py -n $NUMBER -t $TIME -o full -r 10.12.0.10 -m single -g 5
python main.py -n $NUMBER -t $TIME -o full -r 10.12.0.20 -m single -g 5
python main.py -n $NUMBER -t $TIME -o full -r 10.12.0.11 -m single -g 5
python main.py -n $NUMBER -t $TIME -o full -r 10.12.0.18 -m single -g 5
python main.py -n $NUMBER -t $TIME -o full -r 10.12.0.2 -m single -g 5

python main.py -n $NUMBER -t $TIME -o full -r 10.12.0.10 -m single -g 5
python main.py -n $NUMBER -t $TIME -o full -r 10.12.0.20 -m single -g 5
python main.py -n $NUMBER -t $TIME -o full -r 10.12.0.11 -m single -g 5
python main.py -n $NUMBER -t $TIME -o full -r 10.12.0.18 -m single -g 5
python main.py -n $NUMBER -t $TIME -o full -r 10.12.0.2 -m single -g 5

python main.py -n $NUMBER -t $TIME -o full -r 10.12.0.10 -m single -g 5
python main.py -n $NUMBER -t $TIME -o full -r 10.12.0.20 -m single -g 5
python main.py -n $NUMBER -t $TIME -o full -r 10.12.0.11 -m single -g 5
python main.py -n $NUMBER -t $TIME -o full -r 10.12.0.18 -m single -g 5
python main.py -n $NUMBER -t $TIME -o full -r 10.12.0.2 -m single -g 5

python main.py -n $NUMBER -t $TIME -o full -r 10.12.0.10 -m single -g 5
python main.py -n $NUMBER -t $TIME -o full -r 10.12.0.20 -m single -g 5
python main.py -n $NUMBER -t $TIME -o full -r 10.12.0.11 -m single -g 5
python main.py -n $NUMBER -t $TIME -o full -r 10.12.0.18 -m single -g 5
python main.py -n $NUMBER -t $TIME -o full -r 10.12.0.2 -m single -g 5

mv var/log var/logmob2