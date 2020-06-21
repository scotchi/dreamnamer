#!/usr/bin/env ruby

require 'json'
require 'ferret'

data = File.expand_path('../tv_series_ids_06_20_2020.json',  __FILE__)
series = File.read(data).split("\n").map { |l| JSON.parse(l) }

index = Ferret::Index::Index.new

series.each do |s|
  doc = Ferret::Document.new
  doc['original_name'] = Ferret::Field.new(s['original_name'], s['popularity'])
  index << doc
end

# Output all results
#
# index.search(QUERY).hits.each do |hit|
#  puts "#{index.doc(hit.doc)[:original_name]}: #{hit.score}"
# end

ARGV.each do |query|
  hit = index.search(query.gsub(/\W/, ' ')).hits.first
  puts index.doc(hit.doc)[:original_name]
end


