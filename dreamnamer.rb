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

def print_hit(index, query, hit)
  return unless hit
  puts "#{hit.score < 10 ? '* ' : ''}#{query}: #{index.doc(hit.doc)[:original_name]}: #{hit.score}"
  # puts index.doc(hit.doc)[:original_name]
end


ARGV.each do |query|
  hits = index.search(query.gsub(/\W/, ' '))
  if ENV['SCORES']
    hits.each { |hit| print_hit(index, query, hit) }
  else
    print_hit(index, query, index.search(query.gsub(/\W/, ' ')).hits.first)
  end
end


