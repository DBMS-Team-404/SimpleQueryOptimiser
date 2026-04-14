SELECT u1.name, u2.name, o1.amount, o2.amount, p.name
FROM users AS u1
JOIN users AS u2 ON u1.age = u2.age
JOIN orders AS o1 ON u1.id = o1.user_id
JOIN orders AS o2 ON u1.id = o2.user_id
JOIN products AS p ON o1.order_date = p.id
WHERE u1.age > 20 
  AND u2.age < 50
  AND p.category_id = 5
  AND o1.amount > 1000
  AND o2.amount > 100
  AND u1.age > u2.age
  AND o1.amount > o2.amount;