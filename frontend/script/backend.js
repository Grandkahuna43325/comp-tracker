import { Comp } from "./structs.js";

let data;
export let comps;

export function get_data() {
  (async () => {
    data = await getData("http://localhost:8080/data");
    comps = Comp.fromJSON(data);
  })();
}

export async function getData(urll) {
  try {
    const response = await fetch(urll);
    if (!response.ok) {
      throw new Error(`Response status: ${response.status}`);
    }

    return await response.json();
  } catch (error) {
    console.log(error.message);
    return null;
  }
}


