SELECT name, amount FROM orders JOIN users ON orders.user_id = users.id WHERE age > 18 GROUP BY name;
