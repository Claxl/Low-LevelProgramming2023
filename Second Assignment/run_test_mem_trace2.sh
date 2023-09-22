#!/bin/bash


sizes=("128" "256" "512" "1024" "2048" "4096")
mappings=("dm" "fa")
orgs=("uc" "sc")

OUT=mem_trace2.out
echo $'\n' > $OUT

for size in "${sizes[@]}"; do
  for mapping in "${mappings[@]}"; do
    for org in "${orgs[@]}"; do
      echo "./cache_sim $size $mapping $org"
      ./cache_sim $size $mapping $org
      echo $'\n\n'
    done >> $OUT
  done
done
