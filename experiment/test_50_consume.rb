#!/usr/bin/env ruby

`amqp-declare-queue -q queue --server localhost --port 5672`

c = (1..50).map do
  Thread.new do
    `amqp-consume --server localhost --port 5672 -q queue cat`
  end
end

c.each {|pp| pp.join()}

