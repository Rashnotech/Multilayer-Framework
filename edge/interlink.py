#!/usr/bin/env python3
"""a mdoule that communicate between edges node and central sys"""
import pika



def send_message_to_central(message):
    connection = pika.BlockingConnection(pika.ConnectionParameters('localhost'))
    channel = connection.channel()
    
    channel.queue_declare(queue='threat_queue')
    channel.basic_publish(exchange='', routing_key='threat_queue', body=message)
    print(f"Sent: {message}")
    connection.close()



def receive_messages():
    connection = pika.BlockingConnection(pika.ConnectionParameters('localhost'))
    channel = connection.channel()

    channel.queue_declare(queue='threat_queue')

    def callback(ch, method, properties, body):
        print(f"Received threat report: {body}")

    channel.basic_consume(queue='threat_queue', on_message_callback=callback, auto_ack=True)
    print('Waiting for messages...')
    channel.start_consuming()
