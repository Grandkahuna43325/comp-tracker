async function run () {
  let i = await fetch("http://localhost:8080/hello", {
    method: "POST",
    body: JSON.stringify({
      userId: 1,
      title: "Fix my bugs",
      completed: false
    }),
    headers: {
      "Content-type": "application/json; charset=UTF-8"
    }
  });

  console.log(JSON.stringify(i));

}
