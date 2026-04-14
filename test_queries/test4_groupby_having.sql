SELECT user_id
FROM users
JOIN orders ON users.id = orders.user_id
GROUP BY user_id
HAVING amount > 100;
