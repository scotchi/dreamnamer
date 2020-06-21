#!/usr/bin/env ruby

require 'json'
require 'ferret'

QUERY = ARGV.shift.gsub(/\W/, ' ')

series = File.read('tv_series_ids_06_20_2020.json').split("\n").map { |l| JSON.parse(l) }

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

hit = index.search(QUERY).hits.first
puts index.doc(hit.doc)[:original_name]


