
const nav = document.getElementsByTagName("nav")[0];

export function nav_next() {
  let items = Array.from(nav.children);
  let index = items.findIndex((c) => c.classList.contains("nav-block-active"));
  let nextIndex = (index + 1) % items.length;

  items[index]?.classList.remove("nav-block-active");
  items[nextIndex].classList.add("nav-block-active");
}

export function nav_prev() {
  let items = Array.from(nav.children);
  let index = items.findIndex((c) => c.classList.contains("nav-block-active"));
  let nextIndex = index <= 0 ? items.length - 1 : index - 1;

  items[index]?.classList.remove("nav-block-active");
  items[nextIndex].classList.add("nav-block-active");
}

