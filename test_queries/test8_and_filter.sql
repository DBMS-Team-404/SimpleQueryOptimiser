SELECT name, amount
FROM users
JOIN orders ON users.id = orders.user_id
WHERE age > 18 AND amount > 500;
