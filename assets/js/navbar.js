// When the user scrolls the page, execute myFunction
window.onscroll = function() {
  scrollFunction()
};

// Get the navbar
var navbar = document.getElementById("navbar");

// Get the offset position of the navbar
var sticky = navbar.offsetTop;

// Add the sticky class to the navbar when you reach its scroll position. Remove "sticky" when you leave the scroll position
function scrollFunction() {
  if (window.pageYOffset > sticky) {
    navbar.classList.add("default")
  } else {
    navbar.classList.remove("default");
  }
}

// set the image height to viewport height
// This is to prevent jarring background scaling when using background-size: cover; on mobile browsers that hide the address bar on scroll
function setHeight(){
  document.querySelector('.heroImage').style.height = window.innerHeight + "px";
  console.log(window.innerHeight);
}
