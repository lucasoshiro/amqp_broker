#!/usr/bin/env ruby

`amqp-declare-queue -q queue --server localhost --port 5672`

Thread.new do
  (1...25000).each do
    ['melao', 'banana', 'goiana', 'jaca', 'pera', 'framboeesa'].each do
      |fruta|
      `amqp-publish --server localhost --port 5672 -r queue -b #{fruta}`
    end
  end
end

p.each {|pp| pp.join()}

